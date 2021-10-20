import { async, ComponentFixture, TestBed } from '@angular/core/testing';

import { FaceParaComponent } from './face-para.component';

describe('FaceParaComponent', () => {
  let component: FaceParaComponent;
  let fixture: ComponentFixture<FaceParaComponent>;

  beforeEach(async(() => {
    TestBed.configureTestingModule({
      declarations: [ FaceParaComponent ]
    })
    .compileComponents();
  }));

  beforeEach(() => {
    fixture = TestBed.createComponent(FaceParaComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
