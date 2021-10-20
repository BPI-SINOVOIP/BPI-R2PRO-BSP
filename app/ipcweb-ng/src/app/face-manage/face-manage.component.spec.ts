import { async, ComponentFixture, TestBed } from '@angular/core/testing';

import { FaceManageComponent } from './face-manage.component';

describe('FaceManageComponent', () => {
  let component: FaceManageComponent;
  let fixture: ComponentFixture<FaceManageComponent>;

  beforeEach(async(() => {
    TestBed.configureTestingModule({
      declarations: [ FaceManageComponent ]
    })
    .compileComponents();
  }));

  beforeEach(() => {
    fixture = TestBed.createComponent(FaceManageComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
