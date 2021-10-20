import { async, ComponentFixture, TestBed } from '@angular/core/testing';

import { FaceConfigComponent } from './face-config.component';

describe('FaceConfigComponent', () => {
  let component: FaceConfigComponent;
  let fixture: ComponentFixture<FaceConfigComponent>;

  beforeEach(async(() => {
    TestBed.configureTestingModule({
      declarations: [ FaceConfigComponent ]
    })
    .compileComponents();
  }));

  beforeEach(() => {
    fixture = TestBed.createComponent(FaceConfigComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
